<?php
// $Id$

/**
 * Handle a random sort.
 */
class views_php_handler_sort extends views_handler_sort {

  /**
   * Only let users with use PHP permissions set/modify this handler.
   */
  function php_check_access(&$element) {
    if (!user_access('use PHP for settings')) {
      $element['#disabled'] = TRUE;
      $element['#value'] = $element['#default_value'];
      $element['#description'] .= ' <strong>' . t('Note: you do not have permission to modify this.') . '</strong>';
    }
  }

  /**
   * Implements views_handler#can_expose().
   */
  function can_expose() { return FALSE; }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_static'] = array('default' => '');
    $options['php_sort'] = array('default' => '');
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_static'] = array(
      '#type' => 'textarea',
      '#title' => t('Static code'),
      '#default_value' => $this->options['php_static'],
      '#rows' => 5,
      '#description' =>
        t('Code to run right before execution of the view. May be useful to define functions to be re-used in the value and/or output code.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    $this->php_check_access($form['php_static']);
    $form['php_sort'] = array(
      '#type' => 'textarea',
      '#title' => t('Sort comparison code'),
      '#default_value' => $this->options['php_sort'],
      '#rows' => 5,
      '#description' =>
        t('The comparison code must return an integer less than, equal to, or greater than zero if the first row should respectively appear before, stay where it was compared to, or appear after the second row.')
        .' '.t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.')
        .' '.t('Available variables:').'<br/>'
        .t('$data: contains the retrieved record from the database (e.g. $data->nid).').'<br/>'
        .t('$static: can be used to store reusable data per row.')
      ,
    );
    $this->php_check_access($form['php_sort']);
  }

  /**
   * Implements views_handler_sort#query().
   */
  function query() {
    // do nothing -- to override the parent query.
  }

  /**
   *
   * @see views_php_views_pre_execute()
   * @see self::php_post_execute()
   */
  function php_pre_execute() {
    // Ecexute static PHP code.
    if (!empty($this->options['php_static'])) {
      $this->php_static(&$this->php_static_variable);
    }
  }

  /**
   *
   * @see views_php_views_post_execute()
   */
  function php_post_execute() {
    if (!empty($this->options['php_sort'])) {
      usort($this->view->result, array($this, 'php_sort'));
    }
  }

  /**
   * Comparison function to sort the results of a view.
   *
   * @see php_post_execute()
   */
  function php_sort($row1, $row2) {
    ob_start();
    $result = eval($this->options['php_sort']);
    ob_get_clean();
    return (int)$result;
  }
}
